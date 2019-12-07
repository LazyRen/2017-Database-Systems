#2014004893 이대인
use Pokemon;

#disable ONLY_FULL_GROUP_BY mode
set sql_mode=(SELECT REPLACE(@@sql_mode,'ONLY_FULL_GROUP_BY',''));

#1
select name from Trainer
where hometown = 'Blue City';

#2
select name from Trainer
where hometown = 'Brown City' or hometown = 'Rainbow City';

#3
select name, hometown from Trainer
where name like 'a%' or name like 'e%' or name like 'i%' or name like 'o%' or name like 'u%';

#4
select name as Pokemon from Pokemon
where type = 'Water';

#5
select distinct type from Pokemon;

#6
select name from Pokemon order by name asc;

#7
select name from Pokemon where name like '%s';

#8
select name from Pokemon where name like '%e%s';

#9
select name from Pokemon
where name like 'a%' or name like 'e%' or name like 'i%' or name like 'o%' or name like 'u%';

#10
select type, count(type) as '# of Pokemon' from Pokemon group by type;

#11
select nickname from CatchedPokemon order by level desc limit 3;

#12
select avg(level) as 'Average Level' from CatchedPokemon;

#13
select (max(level) - min(level)) as 'Max Diff. of Level' from CatchedPokemon;

#14
select count(*) as '# of Pokemon' from Pokemon where name between 'b' and 'f' and name <> 'f';

#15
select count(*) as '# of Pokemon' from Pokemon where type not in ('Fire', 'Grass', 'Water', 'Electric');

#16
select Trainer.name as 'Trainer', Pokemon.name as 'Pokemon', nickname from CatchedPokemon
join Trainer on CatchedPokemon.owner_id = Trainer.id
join Pokemon on CatchedPokemon.pid = Pokemon.id
where nickname like '% %' order by Trainer.name, Pokemon.name;

#17
select Trainer.name from Trainer
where Trainer.id in (
	select CatchedPokemon.owner_id from CatchedPokemon
	join Pokemon on CatchedPokemon.pid = Pokemon.id and Pokemon.type = 'Psychic');

#18
select Trainer.name, Trainer.hometown from Trainer
join CatchedPokemon on Trainer.id = CatchedPokemon.owner_id
group by Trainer.id order by avg(CatchedPokemon.level) desc limit 3;


#19
select Trainer.name, count(*) as '# of Pokemon' from Trainer
join CatchedPokemon on Trainer.id = CatchedPokemon.owner_id
group by Trainer.id order by count(*) desc, Trainer.name desc;

#20
select Pokemon.name, level from Gym
join CatchedPokemon on Gym.leader_id = CatchedPokemon.owner_id and Gym.city = 'Sangnok City'
join Pokemon on CatchedPokemon.pid = Pokemon.id
order by level;

#21
#sql_mode=only_full_group_by ERROR
select Pokemon.name, count(CatchedPokemon.id) as '# of Catched' from Pokemon
left join CatchedPokemon on Pokemon.id = CatchedPokemon.pid
group by Pokemon.name order by count(CatchedPokemon.id) desc;

#22
select Pokemon.name from Evolution, Pokemon
where Evolution.before_id = (select Evolution.after_id from Evolution, Pokemon
	where Evolution.before_id = Pokemon.id and Pokemon.name = 'Charmander')
and Pokemon.id = Evolution.after_id;

#23
select distinct Pokemon.name from Pokemon
join CatchedPokemon on Pokemon.id = CatchedPokemon.pid
where Pokemon.id between 0 and 30 order by Pokemon.name;

#24
#sql_mode=only_full_group_by ERROR
select Trainer.name, Pokemon.type from Trainer
join CatchedPokemon on Trainer.id = CatchedPokemon.owner_id
join Pokemon on CatchedPokemon.pid = Pokemon.id
group by Trainer.id having count(distinct Pokemon.type) = 1;

#25
select Trainer.name, Pokemon.type, count(Pokemon.type) as '# of Pokemon' from Trainer
join CatchedPokemon on Trainer.id = CatchedPokemon.owner_id
join Pokemon on Pokemon.id = CatchedPokemon.pid
group by Trainer.id, Pokemon.type
order by Trainer.name, Pokemon.type, count(Pokemon.type);

#26
#sql_mode=only_full_group_by ERROR
select Trainer.name, Pokemon.name, count(Pokemon.id) as '# of Catched' from Trainer
join CatchedPokemon on Trainer.id = CatchedPokemon.owner_id
join Pokemon on CatchedPokemon.pid = Pokemon.id
group by Trainer.id having count(distinct Pokemon.id) = 1;

#27
#sql_mode=only_full_group_by ERROR
select Trainer.name, Gym.city from Trainer
join Gym on Trainer.id = Gym.leader_id
join CatchedPokemon on Trainer.id = CatchedPokemon.owner_id
join Pokemon on CatchedPokemon.pid = Pokemon.id
group by Trainer.id having count(distinct Pokemon.type) > 1;

#28
select Trainer.name, sum(CatchedPokemon.level) as 'Total Level' from Trainer
join Gym on Trainer.id = Gym.leader_id
left join CatchedPokemon on Trainer.id = CatchedPokemon.owner_id and level >= 50
group by Trainer.id order by sum(CatchedPokemon.level);

#29
select Pokemon.name from CatchedPokemon
join Pokemon on CatchedPokemon.pid = Pokemon.id
join Trainer on CatchedPokemon.owner_id = Trainer.id and Trainer.hometown = 'Blue City'
where Pokemon.name in (
	select Pokemon.name from CatchedPokemon
	join Pokemon on CatchedPokemon.pid = Pokemon.id
	join Trainer on CatchedPokemon.owner_id = Trainer.id and Trainer.hometown = 'Sangnok City');

#30
select Pokemon.name from Pokemon
join Evolution as E1 on Pokemon.id = E1.after_id
left join Evolution as E2 on E1.after_id = E2.before_id
where E2.before_id is null;
